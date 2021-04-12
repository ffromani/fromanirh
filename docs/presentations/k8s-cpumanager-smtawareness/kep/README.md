# SMT-aware cpu manager policy

## Table of Contents

<!-- toc -->
<!-- /toc -->

## Release Signoff Checklist

Items marked with (R) are required *prior to targeting to a milestone / release*.

- [ ] (R) Enhancement issue in release milestone, which links to KEP dir in [kubernetes/enhancements](https://github.com/kubernetes/enhancements/issues/2404)
- [ ] (R) KEP approvers have approved the KEP status as `implementable`
- [ ] (R) Design details are appropriately documented
- [ ] (R) Test plan is in place, giving consideration to SIG Architecture and SIG Testing input
- [ ] (R) Graduation criteria is in place
- [ ] (R) Production readiness review completed
- [ ] Production readiness review approved
- [ ] "Implementation History" section is up-to-date for milestone
- ~~ [ ] User-facing documentation has been created in [kubernetes/website], for publication to [kubernetes.io] ~~
- [ ] Supporting documentation e.g., additional design documents, links to mailing list discussions/SIG meetings, relevant PRs/issues, release notes

[kubernetes.io]: https://kubernetes.io/
[kubernetes/enhancements]: https://git.k8s.io/enhancements
[kubernetes/kubernetes]: https://git.k8s.io/kubernetes
[kubernetes/website]: https://git.k8s.io/website

## Summary

We propose an additional policy for cpumanager to make the behaviour of latency-sensitive applications more predictable when running on SMT-enabled systems.

## Motivation

Latency-sensitive applications want to have exclusive CPU allocation to have performance isolation and meet their performance requirements.
The static policy of the cpumanager already allows this behaviour.
However, for some classes of these latency-sensitive applications running on simultaneous multithreading (SMT) enabled system, it is also necessary to consider thread affinity, to avoid any possible interference caused by noisy neighborhoods.

### Goals

* Allow the workload to request the core allocation at per-hardware-thread level, avoiding noisy neighbours situations and, as future extension, possibly enabling emulation of non-SMT behaviour on SMT systems. 

## Proposal

### User Stories

#### Containerized Network Functions (CNF)

This class of latency applications benefits of hardware-thread level placement control
To maximise cache efficiency, the workload  wants to be pinned to thread siblings. This is already the default behaviour of the cpumanager static policy, but this enhancement wants to provide stronger guarantees.
The workload guest may wish to avoid thread siblings. This is to avoid noisy neighborhoods, which may have effect on core compute resources or in processor cache interference.
Implementations of this policy proposed here are already found in [external projects](https://github.com/nokia/CPU-Pooler#hyperthreading-support)
or in [OpenStack](https://specs.openstack.org/openstack/nova-specs/specs/mitaka/implemented/virt-driver-cpu-thread-pinning.html),
which is one of the leading platform for VNF (Virtual Network Functions), the previous incarnation of CNF.

### Other?

WRITEME

### Risks and Mitigations

This new policy is opt-in. Users will need to explicitly enable it in their kubelet configuration. The change is very self contained, with little impact in the shared codebase.
The impact in the shared codebase will be addressed enhancing the current testsuite.

| Risk                                                      | Impact        | Mitigation |
| --------------------------------------------------------- | ------------- | ---------- |
| Bugs in the implementation lead to kubelet crash | High | Disable the policy and restart the kubelet. The workload will run but with weaker guarantees - like it was before this change. |


## Design Details

### Proposed Change

We propose to add a new cpumanager policy called `smtaware` which will be a further refinement of the existing static policy. This means the new policy will implement all the behaviour of the static policy, with additional guarantees:
Ensure that containers will always get an even number of physical cores, rounding up. This is done to ensure that a guaranteed container will never share resources to any other container, thus preventing any possible noisy neighborhood.
 On 2-way SMT platforms, like Intel and AMD CPUs, which have 2 hardware threads per physical core, this means that each guaranteed container will have allocated a even number of cores, multiple of 2, rounding up.
Ensure that containers will be admitted by the kubelet iff they require a even number of cores
This is done to make the resource accounting trivially consistent, see below for details.

To illustrate the behaviour of the policy, we will consider the following CPU topology. We will take a cpu package with 16 physical cores, 2-way SMT-capable.

![Example Topology](smtaware-topology.png)

The new `smtaware` policy will allocate cores like the current cpumanager static policy, but avoid physical core sharing.
This means that the allocation will be performed in terms of hardware thread sets (physical cores).  On 2-way SMT platforms, like Intel and AMD CPUs, this means hardware thread pairs.
If the container requires a odd number of cores, the leftover core will be left unallocated, and the policy will guarantee no workload will consume it.

Example: let’s consider a pod with one container requesting 5 cores.

![Example core allocation when request is an odd number of cores](smtaware-allocation-odd-cores.png)


### Test Plan

The implementation PR will extend both the unit test suite and the E2E test suite to cover the policy changes described in this KEP.

### Graduation Criteria

#### Alpha
- [] ???

#### Alpha to Beta Graduation
- [] ???

#### Beta to G.A Graduation
- [] ???

### Upgrade / Downgrade Strategy

We expect no impact. The new policy is opt-in and completely separated by the existing ones.

### Version Skew Strategy

No changes needed

## Production Readiness Review Questionnaire
### Feature enablement and rollback

* **How can this feature be enabled / disabled in a live cluster?**
  - [X] Feature gate (also fill in values in `kep.yaml`).
    - needed? the policy is optin anyway
    - Components depending on the feature gate: kubelet.

* **Does enabling the feature change any default behavior?** No
* **Can the feature be disabled once it has been enabled (i.e. can we rollback the enablement)?** Yes, through kubelet configuration - switch to a different policy.
* **What happens if we reenable the feature if it was previously rolled back?** No changes. Existing container will not see their allocation changed. New containers will.
* **Are there any tests for feature enablement/disablement?** TBD

### Rollout, Upgrade and Rollback Planning

* **How can a rollout fail? Can it impact already running workloads?** Kubelet may fail to start. The kubelet may crash.
* **What specific metrics should inform a rollback?** TBD
* **Were upgrade and rollback tested? Was upgrade->downgrade->upgrade path tested?** Not Applicable.
* **Is the rollout accompanied by any deprecations and/or removals of features,  APIs, fields of API types, flags, etc.?** No.

### Monitoring requirements
* **How can an operator determine if the feature is in use by workloads?**
  - Inspect the kubelet configuration of the nodes.
* **What are the SLIs (Service Level Indicators) an operator can use to determine the health of the service?**
  - [X] Metrics
    - TBD

* **What are the reasonable SLOs (Service Level Objectives) for the above SLIs?** N/A.
* **Are there any missing metrics that would be useful to have to improve observability if this feature?** N/A.


### Dependencies

* **Does this feature depend on any specific services running in the cluster?** Not applicable.

### Scalability

* **Will enabling / using this feature result in any new API calls?** No.
* **Will enabling / using this feature result in introducing new API types?** No.
* **Will enabling / using this feature result in any new calls to cloud provider?** No.
* **Will enabling / using this feature result in increasing size or count of the existing API objects?** No.
* **Will enabling / using this feature result in increasing time taken by any operations covered by [existing SLIs/SLOs][]?** No.
* **Will enabling / using this feature result in non-negligible increase of resource usage (CPU, RAM, disk, IO, ...) in any components?** No.

### Troubleshooting

* **How does this feature react if the API server and/or etcd is unavailable?**: No effect.
* **What are other known failure modes?** TBD
* **What steps should be taken if SLOs are not being met to determine the problem?** N/A

[supported limits]: https://git.k8s.io/community//sig-scalability/configs-and-limits/thresholds.md
[existing SLIs/SLOs]: https://git.k8s.io/community/sig-scalability/slos/slos.md#kubernetes-slisslos

## Implementation History

- 2021-MM-DD: KEP created

