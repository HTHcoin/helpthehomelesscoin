HelpTheHomeless staging tree 0.14.0
===================================

`master:` [![Build Status](https://api.travis-ci.com/HTHcoin/Help-The-Homeless-Coin-0.14.svg?branch=master)](https://travis-ci.com/github/HTHcoin/Help-The-Homeless-Coin-0.14) `develop:` [![Build Status](https://api.travis-ci.com/HTHcoin/Help-The-Homeless-Coin-0.14.svg?branch=code-clean-up)](https://travis-ci.com/github/HTHcoin/Help-The-Homeless-Coin-0.14/branches)

https://hth.world


What is HelpTheHomeless?
------------------------

HelpTheHomeless is an experimental digital currency that enables anonymous, instant
payments to anyone, anywhere in the world. HelpTheHomeless uses peer-to-peer technology
to operate with no central authority: managing transactions and issuing money
are carried out collectively by the network. HelpTheHomeless is the name of the open
source software which enables the use of this currency.

An immediately useable, binary version of the HelpTheHomeless software can be downloaded here: https://github.com/HTHcoin/Help-The-Homeless-Coin-0.14/releases/latest

For more information visit the HTH discord https://discord.gg/VmRQXme.




License
-------

HelpTheHomeless is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is meant to be stable. Development is normally done in separate branches.
[Tags](https://github.com/HTHcoin/Help-The-Homeless-Coin-0.14/tags) are created to indicate new official,
stable release versions of HelpTheHomeless.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing ####

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/qa) are installed) with: `qa/pull-tester/rpc-tests.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

