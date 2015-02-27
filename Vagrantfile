Vagrant.configure("2") do |config|

  config.vm.box = "ubuntu/trusty64"

  config.vm.provider :virtualbox do |vb|
    vb.gui = false
  end

  config.vm.provision "shell", :path => "support/vagrant-provision.sh"
end
