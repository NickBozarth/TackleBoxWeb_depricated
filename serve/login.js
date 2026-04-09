const loginForm = document.getElementById("login-form");
const signupForm = document.getElementById("signup-form");
const loginSignupHeader = document.querySelector("div#login-signup>h3");
var passwordHash = undefined;
var sessionId = undefined;


async function textTosha256B64(text) {
    try {
        const textBuffer = new TextEncoder().encode(text);
        const textHash = await window.crypto.subtle.digest("SHA-256", textBuffer);
        const textHashB64 = btoa(String.fromCharCode(...new Uint8Array(textHash)));
        return Result.ok(textHashB64);
    } catch (err) {
        return Result.err("Failed to sha256 + b64 encode text", text);
    }
}

function isValidUsername(username) {
    if (!username) return false;
    return /^[a-zA-Z0-9]+$/.test(username) && username.length > 7;
}

function containsValidPasswordChars(password) {
    return /^[a-zA-Z0-9\~\!\@\#\$\%\^\&\*\(\)\-\_\=\+\[\{\}\]\\\|\;\:\'\"\,\<\.\>\/\?]+$/.test(password);
}

function isAlphanumeric(string) { return /^[a-zA-Z0-9]+$/.test(string); }
function containsSymbol(string) { return /\W/.test(string); }
function containsAlphaLower(string) { return /[a-z]/.test(string); }
function containsAlphaUpper(string) { return /[A-Z]/.test(string); }

function retrieveUsername(form) {
    const usernameField = form.querySelector("input[name=username]");
    if (usernameField === null) return Result.err("No username field found");
    const username = usernameField.value;
    return Result.ok(username);
}

function retrieveUsernameValidated(form) {
    const usernameRes = retrieveUsername(form);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;

    if (username.length < 8) return Result.err("Username must be at least 8 characters long");
    if (!isAlphanumeric(username)) return Result.err("Username must be alphanumeric");
    return usernameRes;
}


function retrievePassword(form) {
    const passwordField = form.querySelector("input[name=password]");
    if (passwordField === null) return Result.err("No password field found");
    const password = passwordField.value;
    return Result.ok(password);
}

function retrievePasswordValidated(form) {
    const passwordRes = retrievePassword(form);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    if (password.length < 8) return Result.err("Password must be at lease 8 characters long");
    if (!containsValidPasswordChars(password)) return Result.err("Password contains invalid characters");
    if (!containsSymbol(password)) return Result.err("Password must contain at least one symbol");
    if (!containsAlphaLower(password)) return Result.err("Password must contain at least one lowercase character");
    if (!containsAlphaUpper(password)) return Result.err("Password must contain at least one capital character");

    return passwordRes;
}

async function processSignup() {
    // retrieve un & pw
    const usernameRes = retrieveUsernameValidated(signupForm);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;
    const passwordRes = retrievePasswordValidated(signupForm);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    // hash necessary data
    const usernameHashB64Res = await textTosha256B64(username);
    if (usernameHashB64Res.is_err) return usernameHashB64Res;
    const usernameHashB64 = usernameHashB64Res.value;
    const loginHashB64Res = await textTosha256B64(username + password);
    if (loginHashB64Res.is_err) return loginHashB64Res;
    const loginHashB64 = loginHashB64Res.value;

    // TODO parse success and fail values
    // send hashes
    return Result.ok(
        await fetchEncrypted(
            "/signup",
            {
                username_hash: usernameHashB64,
                login_hash: loginHashB64
            }
        )
    );
}

async function processLogin() {
    const usernameRes = retrieveUsername(loginForm);
    if (usernameRes.is_err) return usernameRes;
    const username = usernameRes.value;
    const passwordRes = retrievePassword(loginForm);
    if (passwordRes.is_err) return passwordRes;
    const password = passwordRes.value;

    // hash necessary data
    const usernameHashB64Res = await textTosha256B64(username);
    if (usernameHashB64Res.is_err) return usernameHashB64Res;
    const usernameHashB64 = usernameHashB64Res.value;
    const loginHashB64Res = await textTosha256B64(username + password);
    if (loginHashB64Res.is_err) return loginHashB64Res;
    const loginHashB64 = loginHashB64Res.value;

    passwordHash = await textTosha256B64(password);

    // get the first response from the server
    // data will be structured "[login_id] [saltb64]"
    const resp1Res = await fetchEncrypted(
        "/login",
        {
            username_hash: usernameHashB64,
        }
    );

    if (resp1Res.is_err) return resp1Res;
    let resp1;
    try {
        resp1 = JSON.parse(resp1Res.value);
    } catch (err) {
        return Result.err(resp1Res.value);
    }

    const salt = resp1["salt"];
    const requestId = resp1["request_id"];
    if (salt === undefined || requestId === undefined) return Response.err("resp1 malformed");

    const loginHash2Res = await textTosha256B64(loginHashB64 + salt);
    if (loginHash2Res.is_err) return loginHash2Res;
    const loginHash2 = loginHash2Res.value;
    console.log(loginHash2);
    console.log("what3");

    let resp2Res = await fetchEncrypted(
        "/login",
        {
            request_id: requestId,
            username_hash: usernameHashB64,
            login_hash2: loginHash2,
        }
    )

    if (resp2Res.is_err) return resp2Res;
    let resp2;
    try {
        resp2 = JSON.parse(resp2Res.value);
    } catch (err) {
        console.log(resp2);
        return Result.err(resp2Res.value);
    }

    sessionId = resp2["session_id"];
    if (sessionId === undefined) return Response.err("resp2 malformed");

    return Result.ok("Login Success");
}


loginForm.addEventListener("submit", async event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Signup";
        loginForm.hidden = true;
        signupForm.hidden = false;
        return;
    }

    console.log("what")
    console.log(await processLogin());
    console.log("what2");
});

signupForm.addEventListener("submit", async event => {
    event.preventDefault();

    const button = event.submitter;
    if (button.classList.contains("toggle-form")) {
        loginSignupHeader.innerHTML = "Login";
        loginForm.hidden = false;
        signupForm.hidden = true;
        return;
    }

    console.log(await processSignup());
});