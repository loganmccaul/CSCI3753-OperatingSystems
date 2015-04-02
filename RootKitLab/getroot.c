int main(){
	setreuid(1337,1337);
	printf("%d\n", geteuid());
	execve("/bin/sh", 0, 0);
	return 0;
}
