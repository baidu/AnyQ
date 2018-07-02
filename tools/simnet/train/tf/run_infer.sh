set -e # set -o errexit
set -u # set -o nounset
set -o pipefail 

in_task_type='predict'
in_task_conf='./examples/cnn-pointwise.json'
python tf_simnet.py \
		   --task $in_task_type \
		   --task_conf $in_task_conf

