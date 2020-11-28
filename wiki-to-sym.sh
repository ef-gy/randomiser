#!/bin/bash

verbatim_bank=0
current_bank=0
all_names=""
symname=""
highc=0

namify() {
  echo $@ | tr -d -c 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_'
}

get-name() {
  basen="$(namify $@)"
  sn="_${basen:0:30}"
  n="${sn}"

  while echo "${all_names}" | grep "${n}"; do
    highc=$(($highc + 1))
    n="${sn}_${highc}"
  done >/dev/null

  all_names="${all_names} ${n}"

  symname="${n}"
}

parse-wiki() {
  bank_regex="^==.*Bank ([0-9a-fA-F]+)"

  range_type1_regex='([0-9a-fA-F]{4,5}) +- +([0-9a-fA-F]{4,5}) +: +(.+)$'
  range_type2_regex='([0-9a-fA-F]{4}) +- +(.+)$'
  range_type3_regex='0x([0-9a-fA-F]{4,5}) +to +0x([0-9a-fA-F]{4,5}) += +(.+)$'
  range_type4_regex='0x([0-9a-fA-F]{4,5}) +to +0x-{4,5} += +(.+)$'

  range_start=0
  range_end=0
  range_length=0
  range_comment="unknown"
  range_type=data

  comment="; $@"

  if [[ $@ =~ $bank_regex ]]; then
    verbatim_bank=${BASH_REMATCH[1]}
    current_bank=$(printf "%02x" "0x${BASH_REMATCH[1]}")
    echo "; bank ${current_bank}"
    return
  fi

  if [[ $@ =~ $range_type1_regex ]]; then
    range_start=$(printf "0x%04x" "0x${BASH_REMATCH[1]}")
    range_end=$(printf "0x%04x" "0x${BASH_REMATCH[2]}")
    range_comment="${BASH_REMATCH[3]}"
  elif [[ $@ =~ $range_type2_regex ]]; then
    range_start=$(printf "0x%04x" "0x${BASH_REMATCH[1]}")
    range_comment="${BASH_REMATCH[2]}"
  elif [[ $@ =~ $range_type3_regex ]]; then
    range_start=$(printf "0x%04x" "0x${BASH_REMATCH[1]}")
    range_end=$(printf "0x%04x" "0x${BASH_REMATCH[2]}")
    range_comment="${BASH_REMATCH[3]}"
  elif [[ $@ =~ $range_type4_regex ]]; then
    range_start=$(printf "0x%04x" "0x${BASH_REMATCH[1]}")
    range_comment="${BASH_REMATCH[2]}"
  fi

  if [[ $range_start > 0 ]]; then
    if [[ $range_end > 0 ]]; then
      range_length=$(($range_end - $range_start + 1))
    else
      range_length=1
    fi

    if [[ "$verbatim_bank" = "0" ]]; then
      range_start=$(($range_start % 0x4000))
    else
      range_start=$(($range_start % 0x4000 + 0x4000))
    fi
  fi

  if [[ $range_start > 0 ]]; then
    get-name ${range_comment}

    printf "%02x:%04x %s\n" "0x$current_bank" "$range_start" "${symname}"
  fi

  if [[ $range_length > 0 ]]; then
    comment=$(printf "%02x:%04x .%s:%x\n" "0x$current_bank" "$range_start" "$range_type" "$range_length")
  fi

  echo "${comment}"
}

while read line; do
  parse-wiki "${line}"
done

echo $names

