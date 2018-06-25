#/bin/sh!

source global.conf
source local.defaults

if [ -f local.conf ]; then
    source local.conf
fi

scp global.conf $login_red:$remote_dir

echo "$login_red"
echo "$remote_dir"
echo "$COM1"
