#!/usr/bin/env ruby
require 'socket'
starttime = Process.clock_gettime(Process::CLOCK_MONOTONIC)

s = TCPSocket.new 'localhost', 8989

s.write("/home/liad12345/testfiles/#{ARGV[0]}.txt\n")


# prints to content of the requested file
s.each_line do |line|
    #puts line
end

s.close
endtime = Process.clock_gettime(Process::CLOCK_MONOTONIC)
elapsed = endtime - starttime
puts "#{ARGV[0]} Elapsed #{elapsed}"