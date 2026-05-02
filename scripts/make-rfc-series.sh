#!/usr/bin/env bash
set -euo pipefail

cat <<'EOF'
This repository is for local RFC preparation only.

Kernel patch series should be generated from commits, not maintained forever as
hand-edited patch files.

If you are already inside a Linux kernel tree and your last four commits are the
intended RFC series, use:

  git format-patch --cover-letter -4

If you are not inside a Linux kernel tree:

  1. Create a local branch in a suitable Linux checkout.
  2. Copy or apply the prototype files from this repository into that tree.
  3. Split the work into reviewable commits.
  4. Regenerate patches with:

     git format-patch --cover-letter -4

This script does not submit email, does not invoke mailing-list tools, and does
not run any send flow.

# DO NOT RUN unless intentionally submitting manually in some other workflow:
# git send-email *.patch
# b4 send
EOF
