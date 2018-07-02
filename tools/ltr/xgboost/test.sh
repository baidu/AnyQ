#!/bin/bash

#Get Predictions
#After training, we can use the output model to get the prediction of the test data:

#../../xgboost mushroom.conf task=pred model_in=0002.model
#For binary classification, the output predictions are probability confidence scores in [0,1], corresponds to the probability of the label to be positive.

./xgboost train_parameter.conf task=pred model_in=0010.model
