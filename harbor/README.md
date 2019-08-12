# Install Harbor

## Docker compose

```sh
$ curl -L https://github.com/docker/compose/releases/download/1.24.1/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
$ chmod +x /usr/local/bin/docker-compose
```

## Harbor

### Getting Certificate Authority

Copy keys and certs to `/data/cert/`:

```sh
$ mkdir -p /data/cert/
$ cp -rp /vagrant/config/certs/* /data/cert/
$ chmod 777 -R /data/cert/
```

### Download Harbor

```sh
$ wget https://storage.googleapis.com/harbor-releases/release-1.8.0/harbor-offline-installer-v1.8.2-rc2.tgz
$ tar xvf harbor-offline-installer-v1.8.2-rc2.tgz
$ cd harbor
$ cp -rp /vagrant/config/harbor.yml ./
```

Load Harbor images:

```sh
$ docker load < harbor.v1.8.2.tar.gz
```

### Deploy Harbor

Prepare config and deploy:
```sh
$ ./prepare
$ docker-compose up -d
```

Install with clair and notary:

```sh
$ sudo ./install.sh --with-notary --with-clair
```

### Push Image to Harbor

```sh
$ mkdir -p /etc/docker/certs.d/192.16.35.99
$ cp /vagrant/config/certs/ca.crt /etc/docker/certs.d/192.16.35.99/
```

Login Harbor with Docker:

```sh
$ docker login 192.16.35.99
```

Pull a image, and tag it as `192.16.35.99/library/<image>`:

```sh
$ docker pull alpine:3.7
$ docker tag alpine:3.7 192.16.35.99/library/alpine:3.7
```

Push image to Harbor:

```sh
$ docker push 192.16.35.99/library/alpine:3.7
```

Access Portal `https://192.16.35.99`.

## Pull image from Harbor to Kubernetes cluster

First, copy `ca.key` to `/etc/docker/certs.d/192.16.35.99` on nodes:

```sh
$ mkdir -p /etc/docker/certs.d/192.16.35.99
$ cp /vagrant/harbor/ca.crt /etc/docker/certs.d/192.16.35.99/
```

## Content trust

```sh
$ mkdir -p $HOME/.docker/tls/192.16.35.99:4443/
$ cp /vagrant/config/certs/ca.crt $HOME/.docker/tls/192.16.35.99:4443/
# $ cp /vagrant/harbor/ca.crt $HOME/.docker/tls/192.16.35.99:4443/

$ export DOCKER_CONTENT_TRUST=1
$ export DOCKER_CONTENT_TRUST_SERVER=https://192.16.35.99:4443
$ docker tag alpine:3.7 192.16.35.99/trust/alpine:3.7
$ docker push 192.16.35.99/trust/alpine:3.7
```
