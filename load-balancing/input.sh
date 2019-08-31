#!/bin/bash

CONTENT=${1?need desired content}
TARGET_POD=${2?target PodIP}
CLUSTER_IP=$(kubectl get svc -o json | jq -r '.items[] | select(.metadata.name == "k8s-udpserver-cluster").spec.clusterIP')
PODIPs=$(kubectl get ep -o json | jq -r '.items[] | select(.metadata.name == "k8s-udpserver-cluster").subsets[].addresses[].ip' | tr '\n' ',')

echo "${CLUSTER_IP}" | sudo tee /proc/k8s/clusterIP
echo "${CONTENT},${TARGET_POD}" | sudo tee /proc/k8s/http
echo "${PODIPs}" | sudo tee /proc/k8s/podIP
