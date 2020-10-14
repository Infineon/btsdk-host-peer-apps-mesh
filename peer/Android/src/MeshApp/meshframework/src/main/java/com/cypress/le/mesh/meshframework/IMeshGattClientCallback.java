package com.cypress.le.mesh.meshframework;

interface IMeshGattClientCallback {
    /**
     * Callback invoked upon connection state change
     */
    public void onOtaStatus(byte status, int percent);
}
