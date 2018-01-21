#!/bin/bash

rm tags
rm cscope.out

ctags -Rb
cscope -Rb
