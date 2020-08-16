#!/usr/bin/env ruby

require 'net/http'
require 'nokogiri'

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
    padded = '%26s' % name
    puts "#{padded} \e[92m#{last}\e[0m\n"
end

Dir.chdir(name) do
    old_version = `grep Version: *.spec`
    unless old_version.include?(version)
        system "rpmdev-bumpspec -n #{version} --comment=\"#{version}\" *.spec 2>/dev/null"; done?
        puts "#{name}-#{version}"
    end

    system "git add ."; done?
end
