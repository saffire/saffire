Vagrant.configure("2") do |config|

  config.vm.box = "lucid64"
  config.vm.box_url = 'http://files.vagrantup.com/lucid64.box'


  config.vm.provider :virtualbox do |vb|
    vb.gui = false
  end

  config.vm.provision "shell", :path => "support/vagrant-provision.sh"
end
