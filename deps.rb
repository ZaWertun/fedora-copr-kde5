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

def export_deps(deps)
    puts 'digraph D {'
    # Write human readable names:
    deps.each do |key, _|
        puts "  #{esc(key)} [label=\"#{key}\"];"
    end
    puts
    deps.each do |key, value|
        puts
        value.sort.each do |v|
            next unless deps.key?(v)
            puts "  #{esc(key)} -> #{esc(v)};"
        end
    end
    # Write graph:
    puts '}'
end

$deps = {}
specs = Dir.glob("*/*.spec")
workers = []
specs.each_slice((specs.size/4)+1) do |slice|
    workers << Thread.new do
        slice.each do |p|          
            spec = `rpmspec -P #{p} 2>/dev/null`.split(/\n/)
        
            name = spec.grep(/Name:/).first.gsub(/Name:\s+(.+)/, '\1').strip
            pack = spec.grep(/%package/)
                       .map {|s| s.gsub(/^%package\s+/, '')}
                       .select {|s| !s.start_with? 'debug'}
                       .map {|s| (s =~ /-n\s+(.+)/) ? $1 : "#{name}-#{s}"}
            pack = [name] + pack

            deps = []
            spec.grep(/BuildRequires:/).each do |str|
                str.gsub!(/BuildRequires:\s+(.+)/, '\1').strip
                br = str.split(/\s+/)
                rm = []
                br.each_with_index {|v, i| rm += [i, i+1] if v[0] == '>' }
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
                $deps[p] = deps
            end
        end
    end
end
workers.each(&:join)

$deps = Hash[$deps.sort]
export_deps($deps)
