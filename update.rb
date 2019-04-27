#!/usr/bin/env ruby

name = File.basename(Dir.pwd)
version = ARGV[0]
unless version
    STDERR.puts "Usage: #{__FILE__} <VERSION>"
    exit 1
end

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

old_version = `grep Version: *.spec`
unless old_version.include?(version)
    show "Removing old sources"
    system "rm -v *.tar.*"; done?

    show "Bumping version"
    system "rpmdev-bumpspec -n #{version} --comment=\"#{version}\" *.spec"; done?
end

show "Downloading new sources"
system "spectool -g *.spec"; done?

show "Commiting to GIT"
system "git add . && git commit -m #{name}-#{version}"; done?
