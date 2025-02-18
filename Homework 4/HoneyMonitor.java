public class HoneyMonitor {
    private int counter;
    private final int capacity;
    private boolean isFull = false;

    public HoneyMonitor(int capacity) {
        this.capacity = capacity;
    }

    public synchronized void incrementCounter() {
        while (counter == capacity) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        counter++;
        System.out.printf("Bee %s added honey. %d\n", Thread.currentThread().getName(), counter);

        if (counter == capacity) {
            System.out.println("Pot is full! Waking up the bear...");
            isFull = true;
            notify();
        }
    }

    public synchronized void decrementCounter() {
        while (!isFull) {
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        counter = 0;
        isFull = false;
        System.out.println("Bear ate all the honey!");
        notifyAll();
    }

    public synchronized int getCounter() {
        return counter;
    }
}
