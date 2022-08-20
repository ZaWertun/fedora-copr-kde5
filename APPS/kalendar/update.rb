#!/usr/bin/env ruby

SPEC_FILE = 'kalendar.spec'

lines = File.readlines(SPEC_FILE)
spec_commit  = lines.grep(/%global git_commit\s+(.+)/){$1}.first

commit = nil
date   = nil
Dir.chdir('kalendar') do |_|
    system('git pull --rebase')
    commit = `git log --pretty=format:'%H' -1`
    date   = `git log --date=format:'%Y%m%d' --pretty=format:'%ad' -1`
end

if commit == spec_commit
    puts "Already updated to #{spec_date}, commit #{spec_commit}. Nothing to do."
    exit(1)
end

message = "#{date.sub(/(\d{4})(\d{2})(\d{2})/, '\1-\2-\3')}, commit #{commit}"
puts
puts message

IO.write(SPEC_FILE, File.open(SPEC_FILE) do |f|
    f.read.gsub(/%global (git_date|git_commit)\s+(.+)/) do |m|
        if $1 == 'git_date'
            m.sub($2, date)
        else
            m.sub($2, commit)
        end
    end
  end
)

`rpmdev-bumpspec -c '#{message}' #{SPEC_FILE}`
