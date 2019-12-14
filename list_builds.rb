#!/usr/bin/env ruby

require 'nokogiri'
require 'open-uri'

doc = Nokogiri::HTML(open('https://copr.fedorainfracloud.org/coprs/zawertun/kde/builds/'))
#doc = Nokogiri::HTML(File.read('index.html'))

data = []
doc.css('table.datatable tbody tr').each do |row|
    build, name, version = row.css('td').map{|x| x.text.strip}
    data << [build, name, version]
end

max_size = data.map{|x| x.map(&:size)}.max
data.each do |row|
    puts row.map.with_index {|s, i| s.rjust(max_size[i])}.join("\t")
end
