#!/usr/bin/env ruby

require 'json'
require 'sqlite3'

$db = SQLite3::Database.new 'deps.db'

$db.execute <<-SQL
    create table if not exists DEPS (name string primary key, resolved string); 
SQL

def resolve_dep(name)
    rows = $db.execute('select resolved from DEPS where name = ?', [name])
    return rows[0][0] unless rows.empty?

    cmd = "rpm -q --whatprovides \"#{name}\" 2>/dev/null"
    STDERR.puts(cmd) if ENV['DEBUG'] == '1'
    provides = `#{cmd}`.split(/\n/).first
    unless $?.success?
      cmd = "dnf -y -q provides \"#{name}\" 2>/dev/null |head -n1"
      STDERR.puts(cmd) if ENV['DEBUG'] == '1'
      provides = `#{cmd}`.split(/\n/).first
      if !$?.success? || !provides
          STDERR.puts "Error: #{name} (exit status: #{$?.exitstatus})"
          return nil
      end
    end

    provides.gsub!(/-\d.+/, '')

    begin
        $db.execute('insert into DEPS(name, resolved) values(?, ?)', name, provides)
    rescue SQLite3::ConstraintException
        # ignore
    end

    provides
end

def esc(name)
    name.gsub(/[-+]/, '_')
end

def filter_deps(deps, rest)
    res = []
    deps.each do |key, value|
        next if rest.include?(key)
        res << key if !value.find {|v| deps.key?(v) && !rest.include?(v)}
    end
    res
end

def export_deps(deps)
    puts 'digraph D {'
    # Write human readable names:
    deps.each do |key, _|
        puts "  #{esc(key)} [label=\"#{key}\"];"
    end
    puts
    # Write graph:
    deps.each do |key, value|
        puts
        value.each do |v|
            next unless deps.key?(v)
            puts "  #{esc(key)} -> #{esc(v)};"
        end
    end
    puts '}'
end

$deps = {}
$alias = {}

SPECS = if ARGV.size > 0
  File.readlines(ARGV[0]).map {|l| Dir.glob("#{l.chop}/*.spec").first}
else
    Dir.glob("*/*.spec")
end.freeze

workers = []
SPECS.each_slice((SPECS.size/4)+1) do |slice|
    workers << Thread.new do
        slice.compact.each do |path|
            src = `rpmspec -P #{path} 2>/dev/null`.split(/\n/)
            spec = File.basename(path, '.spec')

            name = src.grep(/Name:/).first.gsub(/Name:\s+(.+)/, '\1').strip
            pack = src.grep(/%package/)
                       .map {|s| s.gsub(/^%package\s+/, '')}
                       .select {|s| !s.start_with? 'debug'}
                       .map {|s| (s =~ /-n\s+(.+)/) ? $1 : "#{name}-#{s}"}
            pack = [name] + pack

            provides = src.grep(/^Provides:/)
                          .grep_v(/plasma-packagestructure/) # fuck it
                          .map {|s| s.gsub(/^Provides:\s+([^\s]+)\s?.*/, '\1')}
            pack += provides

            pack.each {|p| $alias[p] = spec}

            deps = []
            src.grep(/BuildRequires:/).each do |str|
                str.gsub!(/BuildRequires:\s+(.+)/, '\1').strip
                br = str.split(/\s+/)
                rm = []
                br.each_with_index {|v, i| rm += [i, i+1] if v[0] == '>'}
                br.select!.each_with_index {|_, i| !rm.include?(i)}

                br.each do |r|
                    if r =~ /[a-z]+\(.+\)/
                        deps << resolve_dep(r)
                    else
                        deps << r
                    end
                end
            end

            pack.each do |p|
                $deps[p] = deps.compact.sort
            end
        end
    end
end
workers.each(&:join)

$deps = Hash[$deps.sort]

# Filtering out other deps:
$deps.each do |_, deps|
    deps.select!{|d| $deps.key?(d)}
end

# Checking for circular deps:
$circular = {}
$deps.each do |name, deps|
    next if $circular.values.include?(name)

    deps.each do |dep|
        if $deps[dep].include?(name)
            $circular[name] = dep
        end
    end
end
$circular.each do |name, dep|
    STDERR.puts "\e[30m\e[43mWARNING\e[0m: Circular dependency detected #{name} <=> #{dep} !!!"
end

res = {}
rest = []
stage = 0
PREFIX = ENV['PREFIX'] || ''
loop do
    deps = filter_deps($deps, rest)
    break if deps.empty?

    res[stage] = deps.map{|d| $alias[d]}.uniq.sort
    rest += deps
    stage += 1
end

puts JSON.pretty_generate(res)
