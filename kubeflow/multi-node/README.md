# Kubeflow
Instructions for installing Kubeflow on your existing Kubernetes cluster with list of supported options.

## NFS Persistent Volumes

### NFS Server
If an NFS volume is not available to your cluster, you can transform one of the cluster’s nodes into an NFS server with the following commands:

```sh
$ sudo apt-get install -y nfs-common nfs-kernel-server
$ sudo mkdir /nfsroot
```

Than you need to configure /etc/exports to share that directory:

```sh
$ echo "/nfsroot *(rw,no_root_squash,no_subtree_check)" | sudo tee -a /etc/exports
```

Restart your NFS server:

```sh
$ sudo systemctl restart nfs-kernel-server
```

### NFS Client
ach node of the cluster must be able to establish a connection to the NFS server. To enable this, install the following NFS client library on each node:

```sh
$ sudo apt-get -y install nfs-common
```

### NFS Provisioner

#### Install Helm

Install Helm onto master node:

```sh
$ wget https://get.helm.sh/helm-v2.14.3-linux-amd64.tar.gz
$ tar xvf helm-v2.14.3-linux-amd64.tar.gz
$ mv linux-amd64/helm /usr/local/bin/
```

Init Helm server on cluster:

```sh
$ kubectl -n kube-system create sa tiller
$ kubectl create clusterrolebinding tiller \
    --clusterrole cluster-admin \
    --serviceaccount=kube-system:tiller
$ helm init --service-account tiller
```

#### Install NFS Provisioner using Helm
You can install NFS Client Provisioner with Helm:

```sh
$ helm install \
  --name nfs-client-provisioner \
  --set nfs.server=192.16.35.12 \
  --set nfs.path=/nfsroot/kubeflow \
  --set storageClass.name=nfs \
  --set storageClass.defaultClass=true \
  --namespace=kube-system \
  stable/nfs-client-provisioner
```

## Installing command line tools
The following information is useful if you need or prefer to use command line tools for deploying and managing Kubeflow:

```sh
$ wget https://github.com/kubeflow/kubeflow/releases/download/v0.6.1/kfctl_v0.6.1_linux.tar.gz
$ sudo tar -C /usr/local/bin -xzf kfctl_v0.6.1_linux.tar.gz
```

Run the following commands to set up and deploy Kubeflow. The code below includes an optional command to add the binary kfctl to your path. If you don’t add the binary to your path, you must use the full path to the kfctl binary each time you run it:

```sh
$ export CONFIG="https://raw.githubusercontent.com/kubeflow/kubeflow/master/bootstrap/config/kfctl_k8s_istio.yaml"
$ kfctl init kubeflow --config=${CONFIG} -V
$ cd kubeflow
$ kfctl generate all -V
$ kfctl apply all -V
```
> If you want to delete the cluster, you can use this command: `kfctl delete all -V`.