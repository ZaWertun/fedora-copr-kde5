#!/usr/bin/env ruby

require 'net/http'
require 'nokogiri'

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

def last_ver(name)
    uri = URI.parse("https://cgit.kde.org/#{name}.git/refs/")
    http = Net::HTTP.new(uri.host, uri.port)
    http.use_ssl = uri.scheme == 'https'
    res = http.get(uri.request_uri).body
    doc = Nokogiri::HTML(res)
    tags = doc.css('tr th:contains("Tag")').first.parent
    last = tags.next_element
    last.css('td:first-child').first.text
rescue
    nil
end

if (last = last_ver name.gsub(/^kf5-/, ''))
    puts "\e[92mLatest version: #{last}\e[0m\n\n"
end

old_version = `grep Version: *.spec`
unless old_version.include?(version)
    show "Removing old sources"
    system "rm -v *.tar.*"

    show "Bumping version"
    system "rpmdev-bumpspec -n #{version} --comment=\"#{version}\" *.spec"; done?
end

show "Downloading new sources"
system "spectool -g *.spec"; done?

show "Commiting to GIT"
system "git add . && git commit -m \"#{name} updated to #{version}\""; done?
