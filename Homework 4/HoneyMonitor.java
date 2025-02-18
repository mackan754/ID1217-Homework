public class HoneyMonitor {
    private int counter;
    private final int capacity;

    public HoneyMonitor(int capacity) {
        this.capacity = capacity;
    }

    public synchronized void incrementCounter() {
        while (counter == capacity) {  
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }
        }

        counter++;
        System.out.printf("Bee %s added honey. %d\n", Thread.currentThread().getName(), counter);

        if (counter == capacity) {
            System.out.println("Pot is full! Waking up the bear...");
            notifyAll(); 
        }
    }

    public synchronized void decrementCounter() {
        while (counter < capacity) {  
            try {
                wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
                return;
            }
        }

        counter = 0;
        System.out.println("Bear ate all the honey!");
        notifyAll(); 
    }

    public synchronized int getCounter() {
        return counter;
    }
}


