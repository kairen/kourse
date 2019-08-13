# Kubeadm multi-nodes Installation

## Create a cluster
Create nodes as the following commands:

```sh
$ vagrant up
$ vagrant status 
```

Modify the `/etc/hosts` on all nodes, and then add the following content:

```
192.16.35.10    k8s-n1
192.16.35.11    k8s-n2
192.16.35.12    k8s-m1
```

Now, SSH into `k8s-m1`, and then type the following commands:

```sh
$ sudo kubeadm init --apiserver-advertise-address=192.16.35.12 \
  --pod-network-cidr=10.244.0.0/16 \
  --token rlag12.6sd1dhhery5r6fk2 \
  --ignore-preflight-errors=NumCPU 

# Copy kubeconfig
$ mkdir -p $HOME/.kube && \
  sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config && \
  sudo chown $(id -u):$(id -g) $HOME/.kube/config

# Create CNI
$ kubectl apply -f /vagrant/calico.yaml
```

SSH into other nodes, and then type the following commands:

```sh
# Copy from master
$ sudo kubeadm join 192.16.35.12:6443 --token rlag12.6sd1dhhery5r6fk2 \
    --discovery-token-ca-cert-hash sha256:ea1f9e8a715c5fcaf1379073e4f9ed5ea34339398b1fab3bcc2bfe74cc07c6be
```

## Upgrade to v1.15.x

### Upgrading control plane nodes

```sh
$ sudo apt-get update && sudo apt-get install -y kubeadm=1.15.2-00 && \
 sudo apt-mark hold kubeadm
```

Verify that the download works and has the expected version::

```sh
$ kubeadm version
kubeadm version: &version.Info{Major:"1", Minor:"15", GitVersion:"v1.15.2", GitCommit:"f6278300bebbb750328ac16ee6dd3aa7d3549568", GitTreeState:"clean", BuildDate:"2019-08-05T09:20:51Z", GoVersion:"go1.12.5", Compiler:"gc", Platform:"linux/amd64"}
```

On the control plane node, run:

```sh
$ sudo kubeadm upgrade plan
$ sudo kubeadm upgrade apply v1.15.2

# Upgrade nodes
$ sudo kubeadm upgrade node
$ sudo kubeadm upgrade apply v1.15.2
```

Upgrade the kubelet and kubectl on all control plane nodes:

```sh
$ sudo apt-get update && sudo apt-get install -y kubelet=1.15.2-00 kubectl=1.15.2-00 && \
sudo apt-mark hold kubelet kubectl
```

Restart the kubelet:

```sh
$ sudo systemctl restart kubelet
```

### Upgrade worker nodes

Run the following command on nodes(`k8s-n1, k8s-n2`):
```sh
$ sudo apt-get update && sudo apt-get install -y kubeadm=1.15.2-00 && \
 sudo apt-mark hold kubeadm
```

Run the following command on master:

```sh
$ kubectl drain $NODE --ignore-daemonsets
```

Call the following command to upgrade the kubelet configuration on nodes:

```sh
$ sudo kubeadm upgrade node
```

Upgrade the kubelet and kubectl on all nodes:

```sh
$ sudo apt-get update && sudo apt-get install -y kubelet=1.15.2-00 && \
sudo apt-mark hold kubelet 
```

Restart the kubelet:

```sh
$ sudo systemctl restart kubelet
```

Run the following command on master for uncordoning nodes:

```sh
$ kubectl uncordon $NODE
```

## Check cluster

Check version on control plane node:

```sh
$ kubectl version
Client Version: version.Info{Major:"1", Minor:"15", GitVersion:"v1.15.2", GitCommit:"f6278300bebbb750328ac16ee6dd3aa7d3549568", GitTreeState:"clean", BuildDate:"2019-08-05T09:23:26Z", GoVersion:"go1.12.5", Compiler:"gc", Platform:"linux/amd64"}
Server Version: version.Info{Major:"1", Minor:"15", GitVersion:"v1.15.2", GitCommit:"f6278300bebbb750328ac16ee6dd3aa7d3549568", GitTreeState:"clean", BuildDate:"2019-08-05T09:15:22Z", GoVersion:"go1.12.5", Compiler:"gc", Platform:"linux/amd64"}

$ kubectl get no
NAME     STATUS   ROLES    AGE   VERSION
k8s-m1   Ready    master   59m   v1.15.2
k8s-n1   Ready    <none>   57m   v1.15.2
k8s-n2   Ready    <none>   57m   v1.15.2
```