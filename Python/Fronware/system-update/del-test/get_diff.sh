#!/bin/sh

rsync -a --delete fw-v200-old_bk/ fw-v200-old/
rsync -aHv --delete-after fw-v200-new/ fw-v200-old/
