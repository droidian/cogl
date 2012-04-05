#!/bin/bash

. $1

set +m

trap "" ERR
trap "" SIGABRT
trap "" SIGFPE
trap "" SIGSEGV

EXIT=0
WARNING="WARNING: Missing required feature";

if test -f ./test-conformance; then
  TEST_CONFORMANCE=./test-conformance
elif test -f ./test-conformance.exe; then
  TEST_CONFORMANCE=./test-conformance.exe
fi

echo "Key:"
echo "ok = Test passed"
echo "FAIL = Unexpected failure"
echo "fail = Test failed, but it was an expected failure"
echo "PASS! = Unexpected pass"
echo ""

get_status()
{
  case $1 in
      # Special value we use to indicate that the test failed
      # but it was an expected failure so don't fail the
      # overall test run as a result...
      300)
      echo -n "fail";;
      # Special value we use to indicate that the test passed
      # but we weren't expecting it to pass‽
      400)
      echo -n 'PASS!';;

      0)
      echo -n "ok";;

      *)
      echo -n "FAIL";;
  esac
}

run_test()
{
  $($TEST_CONFORMANCE $1 &>.log)
  TMP=$?
  var_name=$2_result
  eval $var_name=$TMP
  if grep -q "$WARNING" .log; then
    if test $TMP -ne 0; then
      eval $var_name=300
    else
      eval $var_name=400
    fi
  else
    if test $TMP -ne 0; then EXIT=$TMP; fi
  fi
}

TITLE_FORMAT="%35s"
printf $TITLE_FORMAT "Test"

if test $HAVE_GL -eq 1; then
  GL_FORMAT=" %6s %8s %7s %6s"
  printf "$GL_FORMAT" "GL+FF" "GL+ARBFP" "GL+GLSL" "GL-NPT"
fi
if test $HAVE_GLES2 -eq 1; then
  GLES2_FORMAT=" %6s %7s"
  printf "$GLES2_FORMAT" "ES2" "ES2-NPT"
fi

echo ""
echo ""

for test in `cat unit-tests`
do
  export COGL_DEBUG=

  if test $HAVE_GL -eq 1; then
    export COGL_DRIVER=gl
    export COGL_DEBUG=disable-glsl,disable-arbfp
    run_test $test gl_ff

    export COGL_DRIVER=gl
    # NB: we can't explicitly disable fixed + glsl in this case since
    # the arbfp code only supports fragment processing so we need either
    # the fixed or glsl vertends
    export COGL_DEBUG=
    run_test $test gl_arbfp

    export COGL_DRIVER=gl
    export COGL_DEBUG=disable-fixed,disable-arbfp
    run_test $test gl_glsl

    export COGL_DRIVER=gl
    export COGL_DEBUG=disable-npot-textures
    run_test $test gl_npot
  fi

  if test $HAVE_GLES2 -eq 1; then
    export COGL_DRIVER=gles2
    export COGL_DEBUG=
    run_test $test gles2

    export COGL_DRIVER=gles2
    export COGL_DEBUG=disable-npot-textures
    run_test $test gles2_npot
  fi

  printf $TITLE_FORMAT "$test:"
  if test $HAVE_GL -eq 1; then
    printf "$GL_FORMAT" \
      "`get_status $gl_ff_result`" \
      "`get_status $gl_arbfp_result`" \
      "`get_status $gl_glsl_result`" \
      "`get_status $gl_npot_result`"
  fi
  if test $HAVE_GLES2 -eq 1; then
    printf "$GLES2_FORMAT" \
      "`get_status $gles2_result`" \
      "`get_status $gles2_npot_result`"
  fi
  echo ""
done

exit $EXIT