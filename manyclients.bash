#!/bin/bash

NUM_INSTANCES=200


for N in {1..50}
do
     # the & sign making it runs them as fast as possible, without waiting for the previous one to finish
    ruby client.rb test$((($N%6)+1)) &
done

# 'wait' command will cause the script to wait for all backround proccesses to finish before exiting
wait
