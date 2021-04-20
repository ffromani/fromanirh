# smt-awareness in cpumanager

- Last updated: April 5, 2021
- contacts: [@fromanirh](https://github.com/fromanirh) [@swatisehgal](https://github.com/swatisehgal)

## Scope

1. (main) latency-sensitive workload running on bare-metal nodes.
2. (secondary) reduce the chance of side-channel attacks using CPU L2/MLC cache.

## Introduction

As in kubernetes 1.21, the cpumanager allows to pin the workload on specific `core`s.
On baremetal nodes however, [what kubernetes calls a `core`](https://kubernetes.io/docs/concepts/configuration/manage-resources-containers/#meaning-of-cpu)
changes meaning drastically depending on the node hardware configuration.
If the SMT is enabled, a `core` is a virtual CPU - a hardware thread; if SMT is disabled, a `core` is a physical CPU.

SMT implementations leverage efficient core resource sharing to improve the throughput.
However, it is possible for different containers (including and most notably non-malicious) to interfere to each other, conflicting to access execution unit, L1 at MLC/L2 caches.
This is an instance of the more general [noisy neighbor problem](https://en.wikipedia.org/wiki/Cloud_computing_issues#Performance_interference_and_noisy_neighbors).

For a certain class of latency sensitive workloads, like realtime or [DPDK applications](https://www.dpdk.org/) the noisy neighbors cause unpredictable and highly undesirable latency spikes.
It becomes thus desirable to enhance cpumanager to avoid these scenarios. Additionally, enabling better control on core sharing reduces the chance of side-channel attacks, thus improving
workload security. 

This [introduction slide deck](https://github.com/fromanirh/fromanirh/blob/main/docs/presentations/k8s-cpumanager-smtawareness/smtawareness-intro.pdf):
- summarizes how the noisy neighbor issue can materialize with the current `static` cpumanager policy. 
- introduces additional cpumanager policies to solve the issue, inspired by
  [already implemented behaviour in openstack](https://specs.openstack.org/openstack/nova-specs/specs/mitaka/implemented/virt-driver-cpu-thread-pinning.html),
  providing examples of the new behaviour.

We believe that introducing additional cpumanager policies is the natural extension path which enables to solve these issues with minimal to none impact to
other workloads, and with minimal changes to the codebase

A more comprehensive presentation, exploring in more details the problem and the proposed solution, and describing the implementation, is expected to be delivered
to sig-node meeting on April 13, 2021. A KEP describing all this work is also in progress.

## Kubernetes enhancement

[Tracked here](https://github.com/kubernetes/enhancements/issues/2625)

## Slides

[Slides for April 20, 2021 sig-node meeting](smt-sware-cpumanger-sig-node-20210420.pdf)

## Attic

- *previous* versions of slides, originally planned for sig-node meeting 20210413 (didn't get to them) [here](OBSOLETE-smt-aware-cpumanager-sig-node-20210413.pdf).
