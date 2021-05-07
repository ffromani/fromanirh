#!/bin/bash
set -ex

KIND_NODE_IMAGE="${KIND_NODE_IMAGE:-quay.io/fromani/kind-node:v1.22-a56d847e32b-smtalign-4}"

cat > kind-config.yaml << EOF
# necessary for conformance
kind: Cluster
apiVersion: kind.x-k8s.io/v1alpha4
networking:
  ipFamily: ipv4
kubeadmConfigPatches:
- |
  apiVersion: kubelet.config.k8s.io/v1beta1
  kind: KubeletConfiguration
  cgroupDriver: "cgroupfs"
  cpuManagerPolicy: static
  cpuManagerReconcilePeriod: 5s
  reservedSystemCPUs: "0"
nodes:
- role: control-plane
- role: worker
- role: worker
EOF
kind create cluster --config kind-config.yaml --image "${KIND_NODE_IMAGE}"

cat > kubelet-config.yaml << EOF
apiVersion: kubelet.config.k8s.io/v1beta1
authentication:
  anonymous:
    enabled: false
  webhook:
    cacheTTL: 0s
    enabled: true
  x509:
    clientCAFile: /etc/kubernetes/pki/ca.crt
authorization:
  mode: Webhook
  webhook:
    cacheAuthorizedTTL: 0s
    cacheUnauthorizedTTL: 0s
cgroupDriver: cgroupfs
clusterDNS:
- 10.96.0.10
clusterDomain: cluster.local
cpuManagerPolicy: static
cpuManagerPolicyOptions:
  - smtalign
cpuManagerReconcilePeriod: 5s
evictionHard:
  imagefs.available: 0%
  nodefs.available: 0%
  nodefs.inodesFree: 0%
evictionPressureTransitionPeriod: 0s
fileCheckFrequency: 0s
healthzBindAddress: 127.0.0.1
healthzPort: 10248
httpCheckFrequency: 0s
imageGCHighThresholdPercent: 100
imageMinimumGCAge: 0s
kind: KubeletConfiguration
logging: {}
nodeStatusReportFrequency: 0s
nodeStatusUpdateFrequency: 0s
reservedSystemCPUs: "0"
rotateCertificates: true
runtimeRequestTimeout: 0s
shutdownGracePeriod: 0s
shutdownGracePeriodCriticalPods: 0s
staticPodPath: /etc/kubernetes/manifests
streamingConnectionIdleTimeout: 0s
syncFrequency: 0s
volumeStatsAggPeriod: 0s
EOF
for CTR_ID in $( docker ps | awk '/kind-worker*/ { print $1 }' ); do
	docker cp ./kubelet-config.yaml ${CTR_ID}:/var/lib/kubelet/config.yaml
	docker exec ${CTR_ID} systemctl restart kubelet
done
