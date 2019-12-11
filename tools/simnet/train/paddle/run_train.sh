set -e # set -o errexit
set -u # set -o nounset
set -o pipefail 

in_task_type='train'
in_conf_file_path='examples/gru_pairwise.json'
python paddle_simnet.py \
		   --task_type $in_task_type \
		   --conf_file_path $in_conf_file_path

