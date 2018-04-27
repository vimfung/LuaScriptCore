package cn.vimfung.luascriptcore;

import android.app.Application;
import android.test.mock.MockContext;

import org.junit.Test;

import cn.vimfung.luascriptcore.test.Env;

import static org.junit.Assert.*;

/**
 * To work on unit tests, switch the Test Artifact in the Build Variants view.
 */
public class ExampleUnitTest {

    @Test
    public void addition_isCorrect() throws Exception {
        assertEquals(4, 2 + 2);
    }

    @Test
    public void raiseException() throws Exception {

        Env.sharedInstance().getContext().evalScript("local p = Person.create(); print(p);");
        assertEquals(4, 2 + 2);

    }
}