require 'socket'

s = TCPSocket.new 'localhost', 8989

s.write("/home/liad12345/testfiles/#{ARGV[0]}.txt\n")

s.each_line do |line|
    puts line
end

s.close