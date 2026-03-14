#!/usr/bin/env bash

# WARNING: BE VERY CAREFUL WHEN USING THIS. IF YOU SELECT THE WRONG DRIVE YOURE DONE FOR

OS_FILE="bin/os.bin"
echo $(pwd)
format_drive(){
  local option
  
  echo "Are you sure you want to write to $1 (Y/n)" >&2
  read option < /dev/tty
  
  if [[ $option == "Y" ]]; then
    echo "Formatting $1 ...">&2
    sudo dd if=/dev/zero of=$1 bs=40M count=1 oflag=dsync
    sudo dd if=$OS_FILE of=$1 status=progress conv=notrunc bs=512 oflag=dsync
    #Using sfdisk to create a valid primary partition at sector 2048 going till 2mb
    sync
  else
    echo "Exiting ..."
  fi
}

check_drive_exists(){
  local drives
  drives=$(lsblk -nd -o NAME,TRAN | grep "usb" | cut -d ' ' -f 1)
  for drive in "${!drives[@]}"; do 
    if [[ drive == "$1" ]]; then
      return
    fi
  done

  return 1
}

find_drive(){
  local drives
  drives=$(lsblk -nd -o NAME,TRAN | grep "usb" | cut -d ' ' -f 1)
  if [[ $drives == "" ]]; then
    echo "Error"
    exit 
  fi

  local chosen_drive
  chosen_drive=$(echo "$drives" | fzf)
  echo $chosen_drive
}

selected_drive="Error"

if [[ -z "$1" ]]; then
  selected_drive=$(find_drive)
else 
  if ! [[ $(check_drive_exists "$1") ]]; then
    selected_drive=$1 
  fi 
fi  


if [[ $selected_drive == "Error" ]]; then
  echo "Error: no usb drive found" >&2
  exit 1
fi

format_drive "/dev/$selected_drive"
