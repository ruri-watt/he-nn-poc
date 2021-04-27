# he-nn-poc
Proof of concept of a neural network capable of providing an encrypted classification of encrypted data. Uses Microsoft Seal Homomorphic encryption library.

# Running POC
There are two options:

## 1. The Vagrant way
Install [vagrant](https://www.vagrantup.com).  
Run the command `vagrant up`

## 2. Using your platform
Basically do what is done in the Vagrantfile:  
Install [Microsoft SEAL](https://github.com/microsoft/SEAL).  
Run the following commands
```
cmake -S. -Bbuild
cd build
make
./he_nn_poc
```