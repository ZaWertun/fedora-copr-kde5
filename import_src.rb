#!/usr/bin/env ruby 

require 'arr-pm'

def show(str)
    puts "\e[93m#{str}:\e[0m"
end

def done?
    if $?.success?
        puts "\e[42m\e[30mDONE\e[0m\n\n"
    else
        puts "\e[41m\e[30mERROR\e[0m\n\n"; exit 2
    end
end

name = ARGV[0]
unless name
    STDERR.puts "Usage: #{__FILE__} <NAME>"
    exit 1
end

show "Cleaning old *.src.rpm"
system "rm -fv #{name}*.src.rpm"; done?

show "Downloading *.src.rpm for #{name}"
system "dnf download --source #{name}"; done?

show "Extracting *.src.rpm"
file = Dir.glob("#{name}*.src.rpm").first
rpm = RPM::File.new(file)
Dir.mkdir(name) unless Dir.exist?(name)
rpm.extract(name)
system "ls -1 #{name}"; done?

show "Commiting to GIT"
version = /-([\d.]+)-/.match(file)[1]
system "git add #{name}/*"
system "git commit -m \"import #{name}-#{version}\""; done?

show "Cleaning *.src.rpm"
system "rm -v #{name}*.src.rpm"
