#!/bin/bash
CLUSTER_IP=${1?need cluster IP}
CONTENT=${2?need desired content}
TARGET_POD=${3?target PodIP}
PODIPs=${4?Pod IP list}

echo "${CLUSTER_IP}" > /proc/k8s/clusterIP
echo "${CONTENT},${TARGET_POD}" > /proc/k8s/http
echo "${PODIPs}" > /proc/k8s/podIP
