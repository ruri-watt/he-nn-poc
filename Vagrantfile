Vagrant.configure("2") do |config|
  config.vm.box = "debian/buster64"

  $script = <<-SCRIPT
  sudo apt install clang cmake git -y
  git clone https://github.com/microsoft/SEAL.git
  cd SEAL
  cmake -S. -Bbuild
  cd build
  make
  sudo make install

  cd
  git clone https://github.com/ruri-watt/he-nn-poc.git
  cd he-nn-poc
  cmake -S. -Bbuild
  cd build
  make
  ./he_nn_poc
  SCRIPT

  config.vm.provision "shell", inline: $script, privileged: false
end
