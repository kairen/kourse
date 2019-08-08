# Kube-ansible
In this section you will deploy a cluster using vagrant and [kube-ansible](https://github.com/kairen/kube-ansible).

**Prerequisites**:
* Ansible version: *v2.5 (or newer)*.
* [Vagrant](https://www.vagrantup.com/downloads.html): >= 2.0.0.
* [VirtualBox](https://www.virtualbox.org/wiki/Downloads): >= 5.0.0.
* Mac OS X need to install `sshpass` tool.

```sh
$ brew install http://git.io/sshpass.rb
```

The getting started guide will use Vagrant with VirtualBox to deploy a Kubernetes cluster onto virtual machines. You can deploy the cluster with a single command:
```sh
$ ./hack/setup-vms
Cluster Size: 1 master, 2 worker.
  VM Size: 1 vCPU, 2048 MB
  VM Info: ubuntu16, virtualbox
  CNI binding iface: eth1
Start to deploy?(y):
```
> * You also can use `sudo ./hack/setup-vms -p libvirt -i eth1` command to deploy a cluster onto KVM.
