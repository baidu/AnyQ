set -e # set -o errexit
set -u # set -o nounset
set -o pipefail 

echo "convert train data"
python ./tools/tf_record_writer.py pointwise ./data/train_pointwise_data ./data/convert_train_pointwise_data 0 32
echo "convert test data"
python ./tools/tf_record_writer.py pointwise ./data/test_pointwise_data ./data/convert_test_pointwise_data 0 32
echo "convert data finish"

in_task_type='train'
in_task_conf='./examples/cnn-pointwise.json'
python tf_simnet.py \
		   --task $in_task_type \
		   --task_conf $in_task_conf

