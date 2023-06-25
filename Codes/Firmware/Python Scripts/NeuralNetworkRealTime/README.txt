In this folder Use NeuralNetworkTest.py for proof of concept realtime 24-10 NN testing

*Note the resistive thresholds for the 24 sensor array need to be experimentally determined and input into the file
to finalize the weights of hidden layer W1, W2 (those can be done by populating np.random.randn(y,x) into 
each of the 24x10 cells of W1 and the 10x10 cells of W2.
Also the map for each gesture (what the experimental


Tenoutput.py is included for future reference. It includes feedforward, filtering, back propagation and sigma parameterization algorithms in python
use accordingly with increasing progress.


BAND test data-21 channels is old data but should be mapped in similar fashion.