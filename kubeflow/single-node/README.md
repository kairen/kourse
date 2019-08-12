# Kubeflow
Instructions for installing Kubeflow on your existing Kubernetes cluster with list of supported options.

## Minikube

First, Create a Kubernetes cluster for Kubeflow:
```sh
$ minikube start --memory=16384 --cpus=4 --kubernetes-version=v1.14.5
```

## Installing command line tools
The following information is useful if you need or prefer to use command line tools for deploying and managing Kubeflow:

```sh
$ wget https://github.com/kubeflow/kubeflow/releases/download/v0.5.1/kfctl_v0.5.1_linux.tar.gz
$ sudo tar -C /usr/local/bin -xzf kfctl_v0.5.1_linux.tar.gz
```

Run the following commands to set up and deploy Kubeflow. The code below includes an optional command to add the binary kfctl to your path. If you don’t add the binary to your path, you must use the full path to the kfctl binary each time you run it:

```sh
$ kfctl init kubeflow -V
$ cd kubeflow && kfctl generate all -V
$ kfctl apply all -V
```

Expose service:

```sh
$ kubectl -n kubeflow port-forward svc/ambassador 8080:80 --address 0.0.0.0
```