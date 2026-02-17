# OpenBSD/sgi is alive! #

Planned since a while - actually since [OpenBSD](https://www.openbsd.org/) retired [the sgi platform](https://www.openbsd.org/sgi.html) in 2021 with [the release of OpenBSD 7.0](https://www.openbsd.org/70.html) - and finally in the works since late November 2022, this is an effort to keep OpenBSD/sgi alive and working.

## Releases ##

For a start the releases usually only provide the kernels because those can be used with [OpenBSD/octeon](https://www.openbsd.org/octeon.html) file systems, too. But for OpenBSD/sgi 7.0 to 7.4 I managed to also build the usual release files (with some exceptions), see the respective release pages for details.

* [OpenBSD/sgi 7.0](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.70.sgi)

* [OpenBSD/sgi 7.1](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.71.sgi)

* [OpenBSD/sgi 7.2](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.72.sgi)

* [OpenBSD/sgi 7.3](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.73.sgi)

* [OpenBSD/sgi 7.4](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.74.sgi)

* [OpenBSD/sgi 7.5](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.75.sgi)

* [OpenBSD/sgi 7.6](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.76.sgi)

* [OpenBSD/sgi 7.7](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.77.sgi)

* [OpenBSD/sgi 7.8](https://github.com/the-machine-hall/openbsd-src/releases/tag/openbsd.78.sgi)

## Branches ##

### Kernel branches ###

For each OpenBSD release I start with the release commit and apply the list of sgi related commits of the last release on top and then try to build each kernel and make adaptations based on changes made in OpenBSD between the last and the current release until the kernel builds work through. Then follows the testing of the kernels on real hardware which sometimes uncovers missing but needed changes I need to find and adapt for the sgi platform, too.

* [sgi-is-alive-at-7.0](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.0)

* [sgi-is-alive-at-7.1](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.1)

* [sgi-is-alive-at-7.2](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.2)

* [sgi-is-alive-at-7.3](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.3)

* [sgi-is-alive-at-7.4](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.4)

* [sgi-is-alive-at-7.5](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.5)

* [sgi-is-alive-at-7.6](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.6)

* [sgi-is-alive-at-7.7](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.7)

* [sgi-is-alive-at-7.8](https://github.com/the-machine-hall/openbsd-src/tree/sgi-is-alive-at-7.8)

### Release branches ###

The release branches since 7.1 are forked from the current sgi-never-retired-* branch after the respective release commit and include changes for the builds to complete successfully and in an adequate time (e.g. omit building LLVM/clang).

* [sgi-release-7.1](https://github.com/the-machine-hall/openbsd-src/tree/sgi-release-7.1)

* [sgi-release-7.2](https://github.com/the-machine-hall/openbsd-src/tree/sgi-release-7.2)

* [sgi-release-7.3](https://github.com/the-machine-hall/openbsd-src/tree/sgi-release-7.3)

* [sgi-release-7.4](https://github.com/the-machine-hall/openbsd-src/tree/sgi-release-7.4)

### Special branch ###

* [sgi-never-retired-3](https://github.com/the-machine-hall/openbsd-src/tree/sgi-never-retired-3)

This branch is special in that it "pretends" that the sgi platform was never retired, by skipping the commits that removed sgi related code, cherry-picking the remaining commits and adapting specific commits that - now that the sgi related code is still there - also need to be applied for the sgi platform. The relevant commits for skipping and adapting are identified in the kernel branches. Instead of the other branches this branch can be used for bisecting (e.g. it was used successfully to find the reason for [#1](https://github.com/the-machine-hall/openbsd-src/issues/1) and it was also helpful in solving [#2](https://github.com/the-machine-hall/openbsd-src/issues/2)).

For each cherry-picked commit the commit message also logs the commit hash of the original commit in [the OpenBSD master branch](https://github.com/openbsd/src/tree/master/) providing a relation between both branches.

Whenever I need to rewrite the history of an already published sgi-never-retired branch, I create a new one and increment the number at the end.

* [sgi-never-retired-2](https://github.com/the-machine-hall/openbsd-src/tree/sgi-never-retired-2) (deprecated)
* [sgi-never-retired](https://github.com/the-machine-hall/openbsd-src/tree/sgi-never-retired) (deprecated)
