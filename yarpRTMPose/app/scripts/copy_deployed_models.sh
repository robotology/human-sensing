#!/bin/bash

usage() { echo "Usage: $0 [-p deployed model path in container] [-c container name]" 1>&2; exit 1; }

SCRIPT_DIR=$( cd -- "$( dirname -- "$BASH_SOURCE[0]}" )" &> /dev/null && pwd )

DEPLOYED_MODEL_PATH="/mmdeploy/rtmpose-ort"
CONTAINER_NAME="yarprtmpose"

while getopts ":p:c:" o; do
	case "${o}" in
		p)
			DEPLOYED_MODEL_PATH=${OPTARG}
			;;
		c)
			CONTAINER_NAME=${OPTARG}
			;;
		*)
			usage
			;;
	esac
done

docker cp ${CONTAINER_NAME}:/$DEPLOYED_MODEL_PATH ${SCRIPT_DIR}/../../download/deployed_models/.
