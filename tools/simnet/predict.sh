set -e # set -o errexit
set -u # set -o nounset
set -o pipefail 

usage()
{
	echo "Usage:"
	echo "sh `basename $0` [paddle/tensorflow]"
}

platform=$1

if [ "paddle" = "$platform" ]; then
    cd train/paddle/
    sh run_infer.sh
    cd ../../
elif [ "tensorflow" = "$platform" ]; then
	cd train/tf/
	sh run_infer.sh
	cd ../../
else
	echo "illegal platform"
	usage
	exit 1
fi
