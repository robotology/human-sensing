#!/bin/bash

yarp detect --write
yarprun --server /container-rtmpose --log &

/bin/bash