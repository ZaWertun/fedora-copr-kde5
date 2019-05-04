#!/usr/bin/env ruby

require 'sqlite3'

$db = SQLite3::Database.new 'deps.db'

$db.execute <<-SQL
    create table if not exists DEPS (name string primary key, resolved string); 
SQL

def resolve_dep(name)
    rows = $db.execute('select resolved from DEPS where name = ?', [name])
    return rows[0][0] unless rows.empty?

    provides = `dnf -C -q provides "#{name}" |head -n1`.split(/\n/).first
    unless provides
        puts "Error: #{name}"
        return nil
    end

    provides.gsub!(/-\d.+/, '')

    $db.execute('insert into DEPS(name, resolved) values(?, ?)', name, provides)
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
specs = Dir.glob("*/*.spec")
workers = []
specs.each_slice((specs.size/4)+1) do |slice|
    workers << Thread.new do
        slice.each do |path|
            src = `rpmspec -P #{path} 2>/dev/null`.split(/\n/)
            spec = File.basename(path, '.spec')

            name = src.grep(/Name:/).first.gsub(/Name:\s+(.+)/, '\1').strip
            pack = src.grep(/%package/)
                       .map {|s| s.gsub(/^%package\s+/, '')}
                       .select {|s| !s.start_with? 'debug'}
                       .map {|s| (s =~ /-n\s+(.+)/) ? $1 : "#{name}-#{s}"}
            pack = [name] + pack

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
                $deps[p] = deps.sort
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

rest = []
stage = 0
PREFIX='kde/'
loop do
    deps = filter_deps($deps, rest)
    break if deps.empty?

    puts if stage > 0
    specs = deps.map{|d| $alias[d]}.uniq.sort
    puts "### Build stage #{stage}:\n\n#{specs.map{|s| PREFIX+s}.join("\n")}"
    rest += deps
    stage += 1
end
