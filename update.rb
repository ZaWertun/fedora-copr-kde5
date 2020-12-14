#!/usr/bin/env ruby

if ARGV.size < 2
    STDERR.puts "Usage: #{__FILE__} <DIR> <VERSION>"
    exit 1
end

name = File.basename(ARGV[0])
version = ARGV[1]

def show(str)
    puts "\e[93m#{str}:\e[0m"
end

def done?
    exit(2) unless $?.success?
end

Dir.chdir(name) do
    old_version = `grep Version: *.spec`
    unless old_version.include?(version)
        system "rpmdev-bumpspec -n #{version} --comment=\"#{version}\" *.spec 2>/dev/null"; done?
        puts "#{name}-#{version}"
    end

    system "git add ."; done?
end
