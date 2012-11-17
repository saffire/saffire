Vagrant::Config.run do |config|
    config.vm.box = 'lucid64'
    config.vm.box_url = 'http://files.vagrantup.com/lucid64.box'

    config.vm.host_name = "vagrant"

    config.vm.provision :shell, :path => "support/vagrant-provision.sh"
end
