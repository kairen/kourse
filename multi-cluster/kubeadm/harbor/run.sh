#!/bin/bash
#
# Setup kubernetes to pull image from the private registry.
# 

set -ex

kubectl create secret docker-registry regcred \
    --docker-server="192.16.35.99" \
    --docker-username=admin \
    --docker-password=r00tme \
    --docker-email=admin@example.com
