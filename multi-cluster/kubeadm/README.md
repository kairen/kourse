# Create a Cluster using kubeadm

Create nodes as the following commands:

```sh
$ vagrant up
$ vagrant status 
```

SSH into `k8s-m1`, and then type the following commands:

```sh
$ sudo su -
$ swapoff -a && sysctl -w vm.swappiness=0
$ kubeadm init --apiserver-advertise-address=192.16.35.12 \
  --pod-network-cidr=10.244.0.0/16 \
  --token rlag12.6sd1dhhery5r6fk2 \
  --ignore-preflight-errors=NumCPU 

# Copy kubeconfig
$ mkdir -p $HOME/.kube && \
  sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config && \
  sudo chown $(id -u):$(id -g) $HOME/.kube/config

# Create CNI
$ kubectl apply -f calico.yaml
```

SSH into other nodes, and then type the following commands:

```sh
$ sudo su -
$ swapoff -a && sysctl -w vm.swappiness=0

# Copy from master
$ kubeadm join 192.16.35.12:6443 --token rlag12.6sd1dhhery5r6fk2 \
    --discovery-token-ca-cert-hash sha256:ea1f9e8a715c5fcaf1379073e4f9ed5ea34339398b1fab3bcc2bfe74cc07c6be
```

