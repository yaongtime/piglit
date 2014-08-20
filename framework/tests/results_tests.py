# Copyright (c) 2014 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

""" Module providing tests for the core module """


import os
import tempfile
import json
import nose.tools as nt
import framework.tests.utils as utils
import framework.results as results
import framework.status as status


def check_initialize(target):
    """ Check that a class initializes without error """
    func = target()
    # Asserting that func exists will fail for Group and TestrunResult which
    # are dict subclasses
    assert isinstance(func, target)


def test_generate_initialize():
    """ Generator that creates tests to initialize all of the classes in core

    In a compiled language the compiler provides this kind of checking, but in
    an interpreted language like python you don't have a compiler test. The
    unit tests generated by this function serve as a similar test, does this
    even work?

    """
    yieldable = check_initialize

    for target in [results.TestrunResult, results.TestResult]:
        yieldable.description = "Test that {} initializes".format(
            target.__name__)
        yield yieldable, target


def test_initialize_jsonwriter():
    """ Test that JSONWriter initializes

    This needs to be handled separately from the others because it requires
    arguments

    """
    with tempfile.TemporaryFile() as tfile:
        func = results.JSONWriter(tfile, file_fsync=False)
        assert isinstance(func, results.JSONWriter)


def test_load_results_folder_as_main():
    """ Test that load_results takes a folder with a file named main in it """
    with utils.tempdir() as tdir:
        with open(os.path.join(tdir, 'main'), 'w') as tfile:
            tfile.write(json.dumps(utils.JSON_DATA))

        results.load_results(tdir)


def test_load_results_folder():
    """ Test that load_results takes a folder with a file named results.json """
    with utils.tempdir() as tdir:
        with open(os.path.join(tdir, 'results.json'), 'w') as tfile:
            tfile.write(json.dumps(utils.JSON_DATA))

        results.load_results(tdir)


def test_load_results_file():
    """ Test that load_results takes a file """
    with utils.resultfile() as tfile:
        results.load_results(tfile.name)


def test_testresult_to_status():
    """ TestResult initialized with result key converts the value to a Status
    """
    result = results.TestResult({'result': 'pass'})
    assert isinstance(result['result'], status.Status), \
        "Result key not converted to a status object"


def test_testrunresult_write():
    """ TestrunResult.write() works

    This tests for a bug where TestrunResult.write() wrote a file containing
    {}, essentially if it dumps a file that is equal to what was provided then
    it's probably working

    """
    with utils.resultfile() as f:
        result = results.load_results(f.name)
        with utils.tempdir() as tdir:
            result.write(os.path.join(tdir, 'results.json'))
            new = results.load_results(os.path.join(tdir, 'results.json'))

    nt.assert_dict_equal(result.__dict__, new.__dict__)


def test_update_results_current():
    """ update_results() returns early when the results_version is current """
    data = utils.JSON_DATA.copy()
    data['results_version'] = results.CURRENT_JSON_VERSION

    with utils.tempdir() as d:
        with open(os.path.join(d, 'main'), 'w') as f:
            json.dump(data, f)

        with open(os.path.join(d, 'main'), 'r') as f:
            base = results.TestrunResult(f)

        res = results.update_results(base, f.name)

    nt.assert_dict_equal(res.__dict__, base.__dict__)


def test_update_results_old():
    """ update_results() updates results

    Because of the design of the our updates (namely that they silently
    incrementally update from x to y) it's impossible to konw exactly what
    we'll get at th end without having tests that have to be reworked each time
    updates are run. Since there already is (at least for v0 -> v1) a fairly
    comprehensive set of tests, this test only tests that update_results() has
    been set equal to the CURRENT_JSON_VERSION, (which is one of the effects of
    runing update_results() with the assumption that there is sufficient other
    testing of the update process.

    """
    data = utils.JSON_DATA.copy()
    data['results_version'] = 0

    with utils.tempdir() as d:
        with open(os.path.join(d, 'main'), 'w') as f:
            json.dump(data, f)

        with open(os.path.join(d, 'main'), 'r') as f:
            base = results.TestrunResult(f)

        res = results.update_results(base, f.name)

    nt.assert_equal(res.results_version, results.CURRENT_JSON_VERSION)
