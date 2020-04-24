#! /bin/sh

# the name of the script without a path
name=`basename $0`
r=n
sed_param="n"
sed_l='s/[A-Z]/\L&/g'
sed_u='s/[a-z]/\U&/g'


# function for printing error messages to diagnostic output
error_msg()
{
	echo "$name: error: $1" 1>&2
}

# function for checking whether the sed_param has been set
check_sed()
{
	if [ $1 != "n" ] #if sed_param has already been set
	then
		echo "Only one parameter is possible" 1>&2; exit 1;	
	fi
}

# function for traversing over directories
traverse()
{
	for file in "$1"/* 
	do
		base_name=`basename $file` #name of a file without a path
		if [ "$base_name" != "*" ]
		then
			newName=`echo $base_name | sed "$sed_param"` #change the name with the given sed parameter
			new_dir="$1/$newName" #create a full path
			if [ "$file" != "$new_dir" ]
			then
				mv $file $new_dir #rename a path
			fi

	               	# if directory then traverse
             	  	if [  -d "$new_dir" ]
			then
                       		traverse "$new_dir"
			fi
                fi
	done
}



# if no arguments given or -h/help is chosen
if [ -z "$1" ]  || [ "$1" = "-h" ] || [ "$1" = "help" ]
then
cat<<EOT
usage:
	$name [-r] [-l|-u] <dir/file names...>
  	$name [-r] <sed pattern> <dir/file names...>
 	$name [-h]
	$name help
$name incorrect syntax example:
	$name -a
EOT
fi

# iterate over parameters
while test "x$1" != "x"
do
	case "$1" in
		-r) r=y;;
		-l) check_sed "$sed_param"; sed_param="$sed_l";;
		-u) check_sed "$sed_param"; sed_param="$sed_u";;
		-*) error_msg "bad option $1"; exit 1;;
		*) if [ "$sed_param" = "n" ]
		then
			sed_param="$1"
			#break
		else
			break
		fi;;
	esac
	shift
done

#test if directory has been entered
if [ -z "$1" ]
then
	error_msg "no directory has been given"
fi

#test if directory/file exists
if [ ! -d "$1" ] && [ ! -f "$1" ]
then
	error_msg "such file or directory does not exist"
fi

#iterating through files and directories
while [ "x$1" != "x" ]
do
	newName=`echo $1 | sed "$sed_param"`
	
	#change name iff they differ
	if [ "$newName" != "$1" ]
	then
        	mv $1 $newName
	fi

	# if directory and recursion
	if [ -d "$newName" ] && [ "$r" = "y" ]
	then
		traverse "$newName"
	fi
	shift
done
