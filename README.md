# My Fields of Mistria Mods

## Note

### Filtering content in `__fiddle__.json`

To filter any key starting with `npcs/adeline`,

```shell
jq -C '. | with_entries(select(.key | startswith("npcs/adeline")))' __fiddle__.json | less -R
```

The `-C` tells jq to output with colors, and `-R` tell less that the intput is raw? So if you want to
dump jq's ouitput to a file, remove `-C` for a raw JSON data without the console's control sequences.

### Dump Related

To repalce a certain substring (e.g. windows' username) in the dumps.

```shell
find . \( -type d -name .git -prune \) -o -type f -print0 | xargs -0 sed --quiet 's/user/jumpyapple/gp'
```

To check the substring in the dumps.

```shell
grep -R "user" .
```
