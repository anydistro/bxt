/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import {
    forwardRef,
    useCallback,
    useEffect,
    useImperativeHandle,
    useRef,
    useState
} from "react";
import { Button, Form, Modal, ModalProps, Select } from "react-daisyui";
import { branches } from "../utils/SectionUtils";
import { createPortal } from "react-dom";
import axios from "axios";

export type ISnapshotModalProps = ModalProps & {
    sections: ISection[];
    sourceBranch?: string;
    targetBranch?: string;
    repository?: string;
    architecture?: string;
};

export default forwardRef<HTMLDialogElement, ISnapshotModalProps>(
    (props, ref) => {
        const [sourceBranch, setSourceBranch] = useState(props.sourceBranch);

        const internalRef = useRef<HTMLDialogElement>(null);
        useImperativeHandle<HTMLDialogElement | null, HTMLDialogElement | null>(
            ref,
            () => internalRef.current
        );

        useEffect(() => {
            if (props.sourceBranch) {
                setSourceBranch(props.sourceBranch);
            } else if (props.sections.at(0)) {
                setSourceBranch(props.sections[0].branch);
            }
        }, [props.sourceBranch, props.sections]);

        const [targetBranch, setTargetBranch] = useState(props.targetBranch);

        useEffect(() => {
            if (props.targetBranch) {
                setTargetBranch(props.targetBranch);
            } else if (props.sections.at(0)) {
                setTargetBranch(props.sections[0].branch);
            }
        }, [props.targetBranch, props.sections]);

        const doSnap = useCallback(
            async (source?: string, target?: string) => {
                if (!source || !target) return;
                try {
                    await axios.post("/api/snap", {
                        source: {
                            branch: source,
                            repository:
                                props.repository ||
                                props.sections[0].repository,
                            architecture:
                                props.architecture ||
                                props.sections[0].architecture
                        },
                        target: {
                            branch: target,
                            repository:
                                props.repository ||
                                props.sections[0].repository,
                            architecture:
                                props.architecture ||
                                props.sections[0].architecture
                        }
                    });
                    internalRef?.current?.close();
                } catch (error) {}
            },
            [props.repository, props.architecture, props.sections]
        );

        return createPortal(
            <Modal ref={internalRef} {...props}>
                <h1 className="text-3xl font-bold">Snapshot</h1>
                <Form>
                    <Form.Label title="Source branch" />
                    <Select
                        value={sourceBranch}
                        size="sm"
                        onChange={(event) => {
                            setSourceBranch(event.target.value);
                        }}
                    >
                        {branches(props.sections || []).map((value) => (
                            <option>{value}</option>
                        ))}
                    </Select>
                </Form>

                <Form>
                    <Form.Label title="Target branch" />
                    <Select
                        value={targetBranch}
                        size="sm"
                        onChange={(event) => {
                            setTargetBranch(event.target.value);
                        }}
                    >
                        {branches(props.sections || []).map((value) => (
                            <option>{value}</option>
                        ))}
                    </Select>
                </Form>
                <Form className="mt-6">
                    <span className="flex space-x-4 justify-end">
                        <Button
                            type="button"
                            color="primary"
                            onClick={(e) => doSnap(sourceBranch, targetBranch)}
                        >
                            Snap
                        </Button>
                    </span>
                </Form>
            </Modal>,
            document.body
        );
    }
);
