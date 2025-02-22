/* 
 * javac HungryBirdsProblem.java
 * java HungryBirdsProblem 10
 */

// Monitor som representerar skålen med maskar
class Bowl {
    private int worms;
    private int capacity;

    public Bowl(int capacity) {
        this.capacity = capacity;
        this.worms = capacity;
    }

    public synchronized void eatWorm(String birdID) throws InterruptedException {
        while (worms == 0) {
            wait();
        }
        // Bird took a worm
        worms--;
        System.out.println("Bird " + birdID + " is eating a worm");

        if (worms == 0) {
            System.out.println(birdID + " found the bowl empty");
            notifyAll();
        }
    }

    public synchronized void refillBowl(String parentBird) throws InterruptedException {
        while (worms > 0) {
            wait();
        }
        worms = capacity;
        System.out.println("Bowl is filled with " + worms + " worms");
        notifyAll();
    }
}

class BabyBird extends Thread {
    private Bowl bowl;
    private int rounds; // Number of rounds baby birds should try to eat

    public BabyBird(Bowl bowl, int rounds, String name) {
        super(name);
        this.bowl = bowl;
        this.rounds = rounds;
    }

    public void run() {
        try {
            for (int i = 0; i < rounds; i++) {
                bowl.eatWorm(getName());
                Thread.sleep(1000);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}

class ParentBird extends Thread {
    private Bowl bowl;
    private int rounds;

    public ParentBird(Bowl bowl, int rounds, String name) {
        super(name);
        this.bowl = bowl;
        this.rounds = rounds;
    }

    public void run() {
        try {
            for (int i = 0; i < rounds; i++) {
                bowl.refillBowl(getName());
                Thread.sleep(1000);
            }
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
    }
}

class HungryBirdsProblem {
    public static void main(String[] args) {
        int rounds = Integer.parseInt(args[0]); // antal cykler
        int W = 5; // Antal maskar i rätten (kapacitet)
        int n = 3; // Antal babyfåglar (konsumenter)

        Bowl bowl = new Bowl(W);

        // Skapa och starta enbart EN ParentBird (producent)
        ParentBird parentBird = new ParentBird(bowl, rounds, "ParentBird");
        parentBird.start();

        // Skapa och starta flera BabyBirds (konsumenter)
        for (int i = 0; i < n; i++) {
            new BabyBird(bowl, rounds, "Babybird-" + i).start();
        }
    }
}