#!/bin/bash

NUM_INSTANCES=50

RUBY_SCRIPT="client.rb test"

# Loop to launch multiple instances concurrently
for ((i = 1; i <= NUM_INSTANCES; i++)); do
    # Execute the Ruby script in the background and redirect output to /dev/null
    ruby "$RUBY_SCRIPT" &> /dev/null &
done


# for N in {1..50}
# do
#      # the & sign making it runs them as fast as possible, without waiting for the previous one to finish
#     ruby client.rb test &
# done

# # 'wait' command will cause the script to wait for all backround proccesses to finish before exiting
# wait
