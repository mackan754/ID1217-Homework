/* 
 * javac BearHoneybeesProblem.java
 * java BearHoneybeesProblem
 */
class Bee extends Thread {

    private final HoneyMonitor honey;

    Bee(HoneyMonitor honey) {
        this.honey = honey;
    }

    @Override
    public void run() {
        while (true) {
            honey.incrementCounter();
            try {
                Thread.sleep((long) (Math.random() * 1000));
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
}

class Bear extends Thread {

    private final HoneyMonitor honey;

    Bear(HoneyMonitor honey) {
        this.honey = honey;
    }

    @Override
    public void run() {
        while (true) {
            honey.decrementCounter();
        }
    }
}

public class BearHoneybeesProblem {

    public static void main(String[] args) {

        int numBees = 10;
        Bee[] bees = new Bee[numBees];
        int pot = 10;
        HoneyMonitor honey = new HoneyMonitor(pot);

        System.out.println("main started\n");

        // Skapa bear
        Bear bear = new Bear(honey);
        bear.start();

        // Skapa bee workers.
        for (int i = 0; i < numBees; i++) {
            System.out.printf("Bee %d created\n.", i);
            bees[i] = new Bee(honey);
            bees[i].start();
        }
    }
}
