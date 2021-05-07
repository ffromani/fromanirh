SMT-aligning cpumanager live demo!
==================================

Try out [SMT-aligning cpumanager](https://github.com/kubernetes/enhancements/pull/2626) on your workstation thanks to [kind >= 0.10](https://kind.sigs.k8s.io/)!

Download [setup-demo.sh](https://raw.githubusercontent.com/fromanirh/fromanirh/main/docs/presentations/k8s-cpumanager-smtawareness/demo/setup-demo.sh) and run it on your host!

Download and watch the [demo](https://raw.githubusercontent.com/fromanirh/fromanirh/main/docs/presentations/k8s-cpumanager-smtawareness/demo/smtalign.cast) using [asciinema](https://asciinema.org/) or see it [online](https://asciinema.org/a/412556)!

`setup-demo.sh` will boot up a 2-worker kind cluster with all the settings enabled to try the SMT-alignment feature.
Unfortunately, because the enhancmement requires a new kubelet config field, we need to run extra steps besides the usual `kind` commandline.
Every step is documented in the `setup-demo.sh` script itself.

Once you have your `kind` cluster running, you can see the negative flow: if the [pod cpu requests cannot be fullfilled with full physical cores](https://raw.githubusercontent.com/fromanirh/fromanirh/main/docs/presentations/k8s-cpumanager-smtawareness/demo/gu-pod-odd.yaml),
the pod will be rejected.

```bash
$ cat gu-pod-odd.yaml 
apiVersion: v1
kind: Pod
metadata:
  name: gu-busybox-odd
  labels:
    app: busybox
spec:
  containers:
  - image: k8s.gcr.io/e2e-test-images/busybox:1.29-1
    command:
      - sleep
      - "36000"
    imagePullPolicy: IfNotPresent
    name: busybox
    resources:
      requests:
        cpu: "1"
        memory: "1G"
      limits:
        cpu: "1"
        memory: "1G"

$ kubectl apply -f gu-pod-odd.yaml 
pod/gu-busybox-odd created

$ kubectl get pods
NAME             READY   STATUS              RESTARTS   AGE
gu-busybox-odd   0/1     SMTAlignmentError   0          6s

$ kubectl describe pod gu-busybox-odd
Name:         gu-busybox-odd
Namespace:    default
Priority:     0
Node:         kind-worker2/
Start Time:   Wed, 05 May 2021 12:49:02 +0200
Labels:       app=busybox
Annotations:  Status:  Failed
Reason:       SMTAlignmentError
Message:      Pod SMT Alignment Error: requested 1 cpus not multiple cpus per core = 2
IP:           
IPs:          <none>
Containers:
  busybox:
    Image:      k8s.gcr.io/e2e-test-images/busybox:1.29-1
    Port:       <none>
    Host Port:  <none>
    Command:
      sleep
      36000
    Limits:
      cpu:     1
      memory:  1G
    Requests:
      cpu:        1
      memory:     1G
    Environment:  <none>
    Mounts:
      /var/run/secrets/kubernetes.io/serviceaccount from kube-api-access-z6892 (ro)
Volumes:
  kube-api-access-z6892:
    Type:                    Projected (a volume that contains injected data from multiple sources)
    TokenExpirationSeconds:  3607
    ConfigMapName:           kube-root-ca.crt
    ConfigMapOptional:       <nil>
    DownwardAPI:             true
QoS Class:                   Guaranteed
Node-Selectors:              <none>
Tolerations:                 node.kubernetes.io/not-ready:NoExecute for 300s
                             node.kubernetes.io/unreachable:NoExecute for 300s
Events:
  Type     Reason             Age   From                   Message
  ----     ------             ----  ----                   -------
  Normal   Scheduled          20s   default-scheduler      Successfully assigned default/gu-busybox-odd to kind-worker2
  Warning  SMTAlignmentError  20s   kubelet, kind-worker2  SMT Alignment Error: requested 1 cpus not multiple cpus per core = 2
```

In the positive flow, [if the container requests CAN be fullfilled with full cores](https://raw.githubusercontent.com/fromanirh/fromanirh/main/docs/presentations/k8s-cpumanager-smtawareness/demo/gu-pod-even.yaml),
the pod is admitted as expected:
```bash
$ cat gu-pod-even.yaml 
apiVersion: v1
kind: Pod
metadata:
  name: gu-busybox-even
  labels:
    app: busybox
spec:
  containers:
  - image: k8s.gcr.io/e2e-test-images/busybox:1.29-1
    command:
      - "/bin/sh"
      - "-c"
      - "grep Cpus_allowed_list /proc/self/status | cut -f2 && sleep 1d"
    imagePullPolicy: IfNotPresent
    name: busybox
    resources:
      requests:
        cpu: "2"
        memory: "1G"
      limits:
        cpu: "2"
        memory: "1G"

$ kubectl apply -f gu-pod-even.yaml 
pod/gu-busybox-even created

$ kubectl describe pod gu-busybox-even
Name:         gu-busybox-even
Namespace:    default
Priority:     0
Node:         kind-worker2/172.18.0.3
Start Time:   Wed, 05 May 2021 12:52:17 +0200
Labels:       app=busybox
Annotations:  Status:  Running
IP:           10.244.1.2
IPs:
  IP:  10.244.1.2
Containers:
  busybox:
    Container ID:  containerd://4586a7c1801317477f7f044cd8c640093b9b87d6043a845fe663c1593d9d1d52
    Image:         k8s.gcr.io/e2e-test-images/busybox:1.29-1
    Image ID:      k8s.gcr.io/e2e-test-images/busybox@sha256:39e1e963e5310e9c313bad51523be012ede7b35bb9316517d19089a010356592
    Port:          <none>
    Host Port:     <none>
    Command:
      /bin/sh
      -c
      grep Cpus_allowed_list /proc/self/status | cut -f2 && sleep 1d
    State:          Running
      Started:      Wed, 05 May 2021 12:52:19 +0200
    Ready:          True
    Restart Count:  0
    Limits:
      cpu:     2
      memory:  1G
    Requests:
      cpu:        2
      memory:     1G
    Environment:  <none>
    Mounts:
      /var/run/secrets/kubernetes.io/serviceaccount from kube-api-access-mt2tj (ro)
Conditions:
  Type              Status
  Initialized       True 
  Ready             True 
  ContainersReady   True 
  PodScheduled      True 
Volumes:
  kube-api-access-mt2tj:
    Type:                    Projected (a volume that contains injected data from multiple sources)
    TokenExpirationSeconds:  3607
    ConfigMapName:           kube-root-ca.crt
    ConfigMapOptional:       <nil>
    DownwardAPI:             true
QoS Class:                   Guaranteed
Node-Selectors:              <none>
Tolerations:                 node.kubernetes.io/not-ready:NoExecute for 300s
                             node.kubernetes.io/unreachable:NoExecute for 300s
Events:
  Type    Reason     Age   From                   Message
  ----    ------     ----  ----                   -------
  Normal  Scheduled  21s   default-scheduler      Successfully assigned default/gu-busybox-even to kind-worker2
  Normal  Pulling    21s   kubelet, kind-worker2  Pulling image "k8s.gcr.io/e2e-test-images/busybox:1.29-1"
  Normal  Pulled     19s   kubelet, kind-worker2  Successfully pulled image "k8s.gcr.io/e2e-test-images/busybox:1.29-1" in 1.613905045s
  Normal  Created    19s   kubelet, kind-worker2  Created container busybox
  Normal  Started    19s   kubelet, kind-worker2  Started container busybox

```

