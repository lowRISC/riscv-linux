#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <asm/irq.h>
#include <asm/csr.h>
#include <asm/sbi.h>
#include <asm/delay.h>

unsigned long timebase;

static DEFINE_PER_CPU(struct clock_event_device, clock_event);

static int riscv_timer_set_next_event(unsigned long delta,
	struct clock_event_device *evdev)
{
	sbi_set_timer(get_cycles() + delta);
	return 0;
}

static int riscv_timer_set_oneshot(struct clock_event_device *evt)
{
	/* no-op; only one mode */
	return 0;
}

static int riscv_timer_set_shutdown(struct clock_event_device *evt)
{
	/* can't stop the clock! */
	return 0;
}

static irqreturn_t timer_interrupt(int irq, void *dev_id)
{
	int cpu = smp_processor_id();
	struct clock_event_device *evdev = &per_cpu(clock_event, cpu);
	evdev->event_handler(evdev);
	return IRQ_HANDLED;
}

static struct irqaction timer_irq = {
	.handler = timer_interrupt,
	.flags = IRQF_TIMER,
	.name = "timer",
};


static cycle_t riscv_rdtime(struct clocksource *cs)
{
	return get_cycles();
}

static struct clocksource riscv_clocksource = {
	.name = "riscv_clocksource",
	.rating = 300,
	.read = riscv_rdtime,
#ifdef CONFIG_64BITS
	.mask = CLOCKSOURCE_MASK(64),
#else
	.mask = CLOCKSOURCE_MASK(32),
#endif /* CONFIG_64BITS */
	.flags = CLOCK_SOURCE_IS_CONTINUOUS,
};

void __init init_clockevent(void)
{
	int cpu = smp_processor_id();
	struct clock_event_device *ce = &per_cpu(clock_event, cpu);

	*ce = (struct clock_event_device){
		.name = "riscv_timer_clockevent",
		.features = CLOCK_EVT_FEAT_ONESHOT,
		.rating = 300,
		.cpumask = cpumask_of(cpu),
		.set_next_event = riscv_timer_set_next_event,
		.set_state_oneshot  = riscv_timer_set_oneshot,
		.set_state_shutdown = riscv_timer_set_shutdown,
	};

	clockevents_config_and_register(ce, sbi_timebase(), 100, 0x7fffffff);
}

void __init time_init(void)
{
	timebase = sbi_timebase();
	lpj_fine = timebase;
	do_div(lpj_fine, HZ);

	clocksource_register_hz(&riscv_clocksource, timebase);
	setup_irq(IRQ_TIMER, &timer_irq);
	init_clockevent();
}
