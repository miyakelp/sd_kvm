#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/kvm.h>
#include <sys/mman.h>
#include <unistd.h>


int main(void) {
  int kvm = open("/dev/kvm", O_RDWR);
  // エラーチェック（以降の処理では省略）
  if (kvm < 0) return 1; 

  int vm = ioctl(kvm, KVM_CREATE_VM, 0);

  /* KVM_INTERRUPTを使う場合はPICは使用不可なので以下をコメントにする */
  ioctl(vm, KVM_CREATE_IRQCHIP);
  ioctl(vm, KVM_CREATE_PIT);

  int vcpu = ioctl(vm, KVM_CREATE_VCPU, 0);
  size_t vcpu_mmap_size = ioctl(kvm, KVM_GET_VCPU_MMAP_SIZE, NULL);
  struct kvm_run *run = mmap(NULL, vcpu_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu, 0);

  unsigned char *mem
    = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS | MAP_NORESERVE,
      -1, 0);
  struct kvm_userspace_memory_region region = {
    .guest_phys_addr = 0x00000000,
    .memory_size = 0x100000,
    .userspace_addr = (long long unsigned int)mem
  };
  ioctl(vm, KVM_SET_USER_MEMORY_REGION, &region);

  int ramfd = open("ram", O_RDONLY);
  read(ramfd, mem, 0x100000);
  close(ramfd);

  struct kvm_sregs sregs;
  ioctl(vcpu, KVM_GET_SREGS, &sregs);
  sregs.cs.base = 0xf0000;
  ioctl(vcpu, KVM_SET_SREGS, &sregs);

  struct kvm_interrupt interrupt = {
    .irq = 0x10
  };


  int running = 1;
  while (running) {
    ioctl(vcpu, KVM_RUN, NULL);

    switch (run->exit_reason) {
      case KVM_EXIT_HLT:
       // printf("KVM_EXIT_HLT\n");
        running = 0;
        break;
      case KVM_EXIT_IO:	/* IO操作 */
	// printf("IO 0x%x, 0x%x\n", run->io.port, run->io.direction);
        if (run->io.port == 0x11 && run->io.direction == KVM_EXIT_IO_OUT) {
          putchar(*(char *)((unsigned char *)run + run->io.data_offset));
	  break;
        }
        if (run->io.port == 0x12 && run->io.direction == KVM_EXIT_IO_OUT) {
          /* KVM_INTERRUPTを使う場合はPICは使用不可なので注意 */
          sleep(1);
	  ioctl(vcpu, KVM_INTERRUPT, &interrupt);
	  break;
	}
	break;
    }
  }

  return 0;
}
