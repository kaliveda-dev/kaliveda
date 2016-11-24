#!/bin/sh
$1 version-info --custom \
--template="#define BZR_BRANCH_NICK \"{branch_nick}\"\n#define BZR_REVISION_ID \"{revision_id}\"\n#define BZR_REVISION_DATE \"{date}\"\n#define BZR_REVISION_NUMBER {revno}\n#define BZR_BRANCH_IS_CLEAN {clean}\n" \
> $2
exit
