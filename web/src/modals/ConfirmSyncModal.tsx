/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { forwardRef } from "react";
import { Button, Modal } from "react-daisyui";

export interface IConfirmSyncModal {
    onConfirm?: () => void;
    onCancel?: () => void;
}

export const ConfirmSyncModal = forwardRef<
    HTMLDialogElement,
    IConfirmSyncModal
>(
    (
        {
            onConfirm,
            onCancel,
            ...props
        }: IConfirmSyncModal & React.ComponentProps<typeof Modal>,
        ref
    ) => {
        return (
            <Modal {...props} backdrop={true} ref={ref}>
                <Modal.Header>Warning</Modal.Header>
                <Modal.Body>
                    Performing a synchronization from the remote repository is a
                    lengthy operation and may disrupt dependencies of overlay
                    packages. Please proceed with caution to avoid unintended
                    side effects.
                </Modal.Body>
                <Modal.Actions>
                    <Button
                        size="sm"
                        onClick={() => {
                            if (onConfirm) onConfirm();
                        }}
                        color="ghost"
                    >
                        Do the sync
                    </Button>

                    <Button
                        size="sm"
                        color="primary"
                        onClick={() => {
                            if (onCancel) onCancel();
                        }}
                    >
                        Cancel
                    </Button>
                </Modal.Actions>
            </Modal>
        );
    }
);
export default ConfirmSyncModal;
